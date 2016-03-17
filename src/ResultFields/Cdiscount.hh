<?hh //strict

namespace ElasticExport\ResultFields;

use Plenty\Modules\DataExchange\Contracts\ResultFields;
use Plenty\Modules\DataExchange\Models\FormatSetting;
use Plenty\Modules\Helper\Services\ArrayHelper;

class Cdiscount extends ResultFields
{
    /*
     * @var ArrayHelper
     */
private ArrayHelper $arrayHelper;

    /**
     * @param ArrayHelper $arrayHelper
     */
    public function __construct(ArrayHelper $arrayHelper)
    {
        $this->arrayHelper = $arrayHelper;
    }

    public function generateResultFields(array<FormatSetting> $formatSettings = []):array<string, mixed>
    {
        $settings = $this->arrayHelper->buildMapFromObjectList($formatSettings, 'key', 'value');

        $itemDescriptionFields = ['urlContent'];
        $itemDescriptionFields[] = ($settings->get('nameId')) ? 'name' . $settings->get('nameId') : 'name1';

        if($settings->get('descriptionType') == 'itemShortDescription')
        {
            $itemDescriptionFields[] = 'shortDescription';
        }

        if ($settings->get('descriptionType') == 'itemDescription' || $settings->get('descriptionType') == 'itemDescriptionAndTechnicalData') {
            $itemDescriptionFields[] = 'description';
        }

        if ($settings->get('descriptionType') == 'technicalData' || $settings->get('descriptionType') == 'itemDescriptionAndTechnicalData') {
            $itemDescriptionFields[] = 'technicalData';
        }

        return
            [
                'itemBase'  =>  [
                    'id',
                    'producer',
                ],

                'itemDescription'   =>  [
                    'params' => [
                        'language' => $settings->get('lang') ? $settings->get('lang') : 'fr',
                    ],
                    'fields' => $itemDescriptionFields,
                ],

                'variationBase' =>  [
                    'id',
                    'model'
                ],

                'variationRetailPrice' => [
                    'price',
                ],

                'variationImageList' => [
                    'params' => [
                        'type' => 'variation',
                    ],
                    'fields' => [
                        'type',
                        'path',
                        'position',
                    ]
                ],

                'variationBarcode' => [
                    'params' => [
                        'barcodeType' => $settings->get('barcode') ? $settings->get('barcode') : 'EAN',
                    ],
                    'fields' => [
                        'code',
                        'barcodeId',
                    ]
                ],

                'variationStandardCategory' =>  [
                    'params'    =>  [
                        'plentyId'  =>  $settings->get('plentyId') ? $settings->get('plentyId') : 1000
                    ],
                    'fields'    =>  [
                        'categoryId',
                    ]
                ]
            ];
    }
}