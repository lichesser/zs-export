<?php

namespace ElasticExport\ResultFields;

use Plenty\Modules\DataExchange\Contracts\ResultFields;
use Plenty\Modules\DataExchange\Models\FormatSetting;
use Plenty\Modules\Helper\Services\ArrayHelper;

/**
 * Class KaufluxDE
 * @package ElasticExport\ResultFields
 */
class KaufluxDE extends ResultFields
{
    /*
     * @var ArrayHelper
     */
    private $arrayHelper;

    /**
     * Billiger constructor.
     * @param ArrayHelper $arrayHelper
     */
    public function __construct(ArrayHelper $arrayHelper)
    {
        $this->arrayHelper = $arrayHelper;
    }

    public function generateResultFields(array $formatSettings = []):array
    {
        $settings = $this->arrayHelper->buildMapFromObjectList($formatSettings, 'key', 'value');

        $itemDescriptionFields = [
            'urlContent',
            'keywords',
        ];

        $itemDescriptionFields[] = ($settings->get('nameId')) ? 'name' . $settings->get('nameId') : 'name1';

        if($settings->get('descriptionType') == 'itemShortDescription'
            || $settings->get('previewTextType') == 'itemShortDescription')
        {
            $itemDescriptionFields[] = 'shortDescription';
        }

        if($settings->get('descriptionType') == 'itemDescription'
            || $settings->get('descriptionType') == 'itemDescriptionAndTechnicalData'
            || $settings->get('previewTextType') == 'itemDescription'
            || $settings->get('previewTextType') == 'itemDescriptionAndTechnicalData')
        {
            $itemDescriptionFields[] = 'description';
        }

        if($settings->get('descriptionType') == 'technicalData'
            || $settings->get('descriptionType') == 'itemDescriptionAndTechnicalData'
            || $settings->get('previewTextType') == 'technicalData'
            || $settings->get('previewTextType') == 'itemDescriptionAndTechnicalData')
        {
            $itemDescriptionFields[] = 'technicalData';
        }

        return [
            'itemBase'=> [
                'id',
                'producerId',
                'storeSpecial',
                'free1',
                'free2',
                'free3',
            ],

            'itemDescription' => [
                'params' => [
                    'language' => $settings->get('lang') ? $settings->get('lang') : 'de',
                ],
                'fields' => $itemDescriptionFields,
            ],

            'variationImageList' => [
                'params' => [
                    'itemImages'                                       => [
                        'type'                 => 'item', // all images
                        'imageType'            => ['internal'],
                        'referenceMarketplace' => $settings->get('referrerId') ? $settings->get('referrerId') : 116,
                    ],
                    'variationImages'                    => [
                        'type'                 => 'variation', // current variation images
                        'imageType'            => ['internal'],
                        'referenceMarketplace' => $settings->get('referrerId') ? $settings->get('referrerId') : 116,
                    ],
                ],
                'fields' => [
                    'type',
                    'path',
                    'position',
                ],
            ],

            'variationRecommendedRetailPrice' => [
				'params' => [
					'referrerId' => $settings->get('referrerId') ? $settings->get('referrerId') : 116,
				],
				'fields' => [
					'price',
				],
            ],

            'variationBase' => [
                'id',
                'availability',
                'attributeValueSetId',
                'model',
                'weightG',
                'vatId',
                'limitOrderByStockSelect',
            ],

            'variationBarcodeList' => [
                'params' => [
                    'barcodeType' => $settings->get('barcode'),
                ],
                'fields' => [
                    'code',
                    'barcodeId',
					'variationId',
					'barcodeType',
					'barcodeName',
                ]
            ],

            'variationRetailPrice' => [
				'params' => [
					'referrerId' => $settings->get('referrerId') ? $settings->get('referrerId') : 116,
				],
				'fields' => [
					'price',
					'vatValue',
				],
            ],

            'variationStandardCategory' => [
                'params' => [
                    'plentyId' => $settings->get('plentyId'),
                ],
                'fields' => [
                    'categoryId',
                    'plentyId',
                    'manually',
                ],
            ],

            'itemPropertyList' => [
                 'itemPropertyId',
                 'propertyId',
                 'propertyValue',
                 'propertyValueType',
            ],

            'itemCrossSellingList' => [
                 'itemId',
                 'crossItemId',
            ],

            'variationMarketStatus' => [
                'params' => [
                    'marketId' => 116,
                ],
                'fields' => [
                    'id',
                    'marketId',
                    'marketAccountId',
                    'initialSku',
                    'sku',
                    'active',
                    'createdTimestamp',
                    'lastExportTimestamp',
                    'deletedTimestamp',
                    'marketStatus',
                    'additionalInformation',
                ],
            ],

            'variationStock' => [
                'params' => [
                    'type' => 'virtual',
                ],
                'fields' => [
                    'stockNet',
                ]
            ]
        ];
    }
}
