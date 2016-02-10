<?hh // strict
namespace ElasticExport\Generators;

use Plenty\Modules\DataExchange\Contracts\CSVGenerator;
use Plenty\Modules\Helper\Services\ArrayHelper;
use Plenty\Modules\Item\DataLayer\Models\Record;
use Plenty\Modules\Item\DataLayer\Models\RecordList;
use Plenty\Modules\DataExchange\Models\FormatSetting;
use ElasticExport\Helper\ElasticExportHelper;

class Billiger extends CSVGenerator
{
	/*
     * @var ElasticExportHelper
     */
    private ElasticExportHelper $elasticExportHelper;

	/*
	 * @var ArrayHelper
	 */
	private ArrayHelper $arrayHelper;

	/**
     * BilligerGenerator constructor.
     * @param ElasticExportHelper $elasticExportHelper
     * @param ArrayHelper $arrayHelper
     */
    public function __construct(ElasticExportHelper $elasticExportHelper, ArrayHelper $arrayHelper)
    {
        $this->elasticExportHelper = $elasticExportHelper;
		$this->arrayHelper = $arrayHelper;
    }

	/**
	 * @param mixed $resultData
	 */
	protected function generateContent(mixed $resultData, array<FormatSetting> $formatSettings = []):void
	{
		if($resultData instanceof RecordList)
		{
			$settings = $this->arrayHelper->buildMapFromObjectList($formatSettings, 'key', 'value');

			$this->setDelimiter(";");

			$this->addCSVContent([
					'aid',
					'name',
					'price',
					'link',
					'brand',
					//'ean',
					'desc',
					'shop_cat',
					//'image',
					'dlv_time',
					'dlv_cost',
					'ppu',
					'mpnr',

			]);

			foreach($resultData as $item)
			{
				$data = [
					'aid' 		=> $item->itemBase->id,
					'name' 		=> $this->elasticExportHelper->getName($item, $settings),
					'price' 	=> number_format($item->variationRetailPrice->price, 2, '.', ''),
					'link' 		=> $this->elasticExportHelper->getUrl($item, $settings, true, false),
					'brand' 	=> $item->itemBase->producer,
					//'image'		=> $this->elasticExportHelper->getImages($item, $settings, 1);
					'desc' 		=> $this->elasticExportHelper->getDescription($item, $settings, 256),
	                'shop_cat' 	=> $this->elasticExportHelper->getCategory($item, $settings),
	                'dlv_time' 	=> $this->elasticExportHelper->getAvailability($item, $settings),
					'dlv_cost' 	=> number_format($this->elasticExportHelper->getShippingCost($item, $settings), 2, ',', ''),
	                'ppu' 		=> $this->elasticExportHelper->getBasePrice($item, $settings),
	                'mpnr' 		=> $item->variationBase->model,
				];

				$this->addCSVContent(array_values($data));
			}
		}
	}
}
